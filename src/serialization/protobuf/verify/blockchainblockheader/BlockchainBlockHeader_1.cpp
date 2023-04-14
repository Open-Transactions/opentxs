// Copyright (c) 2018-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlockchainBlockHeader.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>
#include <cstdint>
#include <limits>

#include "internal/serialization/protobuf/verify/BitcoinBlockHeaderFields.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/BlockchainBlockLocalData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/EthereumBlockHeaderFields.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyBlockchain.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const BlockchainBlockHeader& input, const bool silent) -> bool
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
        case std::numeric_limits<std::uint32_t>::max(): {
            bitcoin = true;
        } break;
        case 5:
        case 6: {
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

auto CheckProto_2(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const BlockchainBlockHeader& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const BlockchainBlockHeader& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
