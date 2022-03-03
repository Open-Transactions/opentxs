// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlockchainExternalAddress.hpp"  // IWYU pragma: associated

#include <cstddef>

#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/BlockchainEnums.pb.h"
#include "serialization/protobuf/BlockchainExternalAddress.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    CHECK_HAVE(data);
    auto count{1};
    auto min = std::size_t{20};
    auto max = std::size_t{20};

    switch (input.type()) {
        case BTOUTPUT_MULTISIG: {
            count = 20;
            min = 33;
            max = 65;
        } break;
        case BTOUTPUT_NULL: {
            min = 0;
            max = MAX_VALID_CONTACT_VALUE;
        } break;
        case BTOUTPUT_P2WSH: {
            min = 32;
            max = 32;
        } break;
        case BTOUTPUT_P2PK: {
            min = 33;
            max = 65;
        } break;
        case BTOUTPUT_P2PKH:
        case BTOUTPUT_P2SH:
        case BTOUTPUT_P2WPKH: {
        } break;
        case BTOUTPUT_UNKNOWN:
        case BTOUTPUT_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    if (count < input.data().size()) {
        FAIL_2("Too many keys", input.data().size());
    }

    for (const auto& data : input.data()) {
        if ((min > data.size()) || (max < data.size())) {
            const auto fail = UnallocatedCString{"invalid data size"};
            FAIL_2(fail.c_str(), data.size())
        }
    }

    return true;
}

auto CheckProto_2(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const BlockchainExternalAddress& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
