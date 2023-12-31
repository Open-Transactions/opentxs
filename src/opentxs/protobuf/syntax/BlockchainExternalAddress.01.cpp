// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainExternalAddress.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainEnums.pb.h>
#include <opentxs/protobuf/BlockchainExternalAddress.pb.h>

#include "internal/util/P0330.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainExternalAddress& input, const Log& log) -> bool
{
    CHECK_HAVE(data);
    auto count{1};
    auto min = 20_uz;
    auto max = 20_uz;

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
            FAIL_2(fail.c_str(), data.size());
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
