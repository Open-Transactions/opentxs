// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PurseExchange.hpp"  // IWYU pragma: associated

#include <CashEnums.pb.h>
#include <Purse.pb.h>
#include <PurseExchange.pb.h>

#include "internal/serialization/protobuf/verify/Purse.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCash.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const PurseExchange& input, const bool silent) -> bool
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

    if (proto::PURSETYPE_NORMAL != incoming.state()) {
        FAIL_1("incorrect request purse state");
    }

    if (proto::PURSETYPE_REQUEST != outgoing.state()) {
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

auto CheckProto_2(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const PurseExchange& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
