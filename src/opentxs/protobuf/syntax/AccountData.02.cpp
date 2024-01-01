// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/AccountData.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/AccountData.pb.h>
#include <opentxs/protobuf/RPCEnums.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_2(const AccountData& input, const Log& log) -> bool
{
    CHECK_IDENTIFIER(id);
    OPTIONAL_NAME(label);
    CHECK_IDENTIFIER(unit);
    CHECK_IDENTIFIER(owner);
    CHECK_IDENTIFIER(issuer);

    switch (input.type()) {
        case ACCOUNTTYPE_NORMAL:
        case ACCOUNTTYPE_ISSUER:
        case ACCOUNTTYPE_BLOCKCHAIN: {
        } break;
        case ACCOUNTTYPE_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
