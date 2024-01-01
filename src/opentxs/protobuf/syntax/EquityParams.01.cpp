// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/EquityParams.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContractEnums.pb.h>
#include <opentxs/protobuf/EquityParams.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const EquityParams& input, const Log& log) -> bool
{
    if (!input.has_type()) { FAIL_1(""); }

    switch (input.type()) {
        case EQUITYTYPE_SHARES: {
        } break;
        case EQUITYTYPE_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
