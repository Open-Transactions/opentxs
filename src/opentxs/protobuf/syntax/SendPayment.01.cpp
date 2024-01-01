// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/SendPayment.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCEnums.pb.h>
#include <opentxs/protobuf/SendPayment.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const SendPayment& input, const Log& log) -> bool
{
    switch (input.type()) {
        case RPCPAYMENTTYPE_BLOCKCHAIN: {
            OPTIONAL_IDENTIFIER(contact);
        } break;
        case RPCPAYMENTTYPE_CHEQUE:
        case RPCPAYMENTTYPE_TRANSFER:
        case RPCPAYMENTTYPE_VOUCHER:
        case RPCPAYMENTTYPE_INVOICE:
        case RPCPAYMENTTYPE_BLINDED: {
            CHECK_IDENTIFIER(contact);
        } break;
        case RPCPAYMENTTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    CHECK_IDENTIFIER(sourceaccount);
    OPTIONAL_NAME(memo);

    switch (input.type()) {
        case RPCPAYMENTTYPE_TRANSFER:
        case RPCPAYMENTTYPE_BLOCKCHAIN: {
            CHECK_IDENTIFIER(destinationaccount);
        } break;
        case RPCPAYMENTTYPE_CHEQUE:
        case RPCPAYMENTTYPE_VOUCHER:
        case RPCPAYMENTTYPE_INVOICE:
        case RPCPAYMENTTYPE_BLINDED: {
            CHECK_EXCLUDED(destinationaccount);
        } break;
        case RPCPAYMENTTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
