// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PurseExchange.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/CashEnums.pb.h>
#include <opentxs/protobuf/Purse.pb.h>
#include <opentxs/protobuf/PurseExchange.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Purse.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCash.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const PurseExchange& input, const Log& log) -> bool
{
    UnallocatedCString inValue{"0"};
    UnallocatedCString outValue{"0"};
    const auto& incoming = input.exchange();
    const auto& outgoing = input.request();

    CHECK_SUBOBJECT_VA(exchange, PurseExchangeAllowedPurse(), inValue);
    CHECK_SUBOBJECT_VA(request, PurseExchangeAllowedPurse(), outValue);

    if (inValue != outValue) {
        FAIL_4(
            "incorrect request purse value", outValue, " expected ", inValue);
    }

    if (incoming.type() != outgoing.type()) {
        FAIL_1("incorrect request purse type");
    }

    if (protobuf::PURSETYPE_NORMAL != incoming.state()) {
        FAIL_1("incorrect request purse state");
    }

    if (protobuf::PURSETYPE_REQUEST != outgoing.state()) {
        FAIL_1("incorrect request purse state");
    }

    if (incoming.notary() != outgoing.notary()) {
        FAIL_1("incorrect request purse notary");
    }

    if (incoming.mint() != outgoing.mint()) {
        FAIL_1("incorrect request purse unit definition");
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
