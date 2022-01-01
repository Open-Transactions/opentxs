// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/protobuf/verify/Purse.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <set>
#include <string>

#include "internal/protobuf/Basic.hpp"
#include "internal/protobuf/verify/Envelope.hpp"      // IWYU pragma: keep
#include "internal/protobuf/verify/SymmetricKey.hpp"  // IWYU pragma: keep
#include "internal/protobuf/verify/Token.hpp"         // IWYU pragma: keep
#include "internal/protobuf/verify/VerifyCash.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/CashEnums.pb.h"
#include "serialization/protobuf/Purse.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

#define PROTO_NAME "purse"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_1(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    switch (input.type()) {
        case CASHTYPE_LUCRE: {
        } break;
        case CASHTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", std::to_string(input.type()));
        }
    }

    auto allowedStates = std::pmr::set<TokenState>{};
    auto validFrom = std::int64_t{Clock::to_time_t(Time::min())};
    auto validTo = std::int64_t{Clock::to_time_t(Time::max())};

    switch (input.state()) {
        case PURSETYPE_REQUEST: {
            allowedStates.insert(TOKENSTATE_BLINDED);

            CHECK_SUBOBJECT(secondarykey, PurseAllowedSymmetricKey());
            CHECK_SUBOBJECT(secondarypassword, PurseAllowedEnvelope());
        } break;
        case PURSETYPE_ISSUE: {
            allowedStates.insert(TOKENSTATE_SIGNED);

            CHECK_SUBOBJECT(secondarykey, PurseAllowedSymmetricKey());
            CHECK_SUBOBJECT(secondarypassword, PurseAllowedEnvelope());
        } break;
        case PURSETYPE_NORMAL: {
            allowedStates.insert(TOKENSTATE_READY);
            allowedStates.insert(TOKENSTATE_SPENT);
            allowedStates.insert(TOKENSTATE_EXPIRED);

            CHECK_EXCLUDED(secondarykey);
            CHECK_EXCLUDED(secondarypassword);
        } break;
        case PURSETYPE_ERROR:
        default: {
            FAIL_2("Invalid state", std::to_string(input.state()));
        }
    }

    CHECK_IDENTIFIER(notary);
    CHECK_IDENTIFIER(mint);
    OPTIONAL_SUBOBJECTS_VA(
        token,
        PurseAllowedToken(),
        input.type(),
        allowedStates,
        value,
        validFrom,
        validTo);

    if (input.totalvalue() != value) {
        FAIL_2("Incorrect value", input.totalvalue());
    }

    if (input.latestvalidfrom() != validFrom) {
        FAIL_2("Incorrect valid from", std::to_string(input.latestvalidfrom()));
    }

    if (input.earliestvalidto() != validTo) {
        FAIL_2("Incorrect valid to", std::to_string(input.earliestvalidto()));
    }

    CHECK_SUBOBJECT(primarykey, PurseAllowedSymmetricKey());
    OPTIONAL_SUBOBJECTS(primarypassword, PurseAllowedEnvelope());

    return true;
}

auto CheckProto_2(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_2(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_3(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_4(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_5(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_6(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_7(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_8(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_9(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_10(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_11(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_12(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_13(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_14(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_15(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_16(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_17(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_18(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_19(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const Purse& input, const bool silent) -> bool
{
    std::string value{"0"};

    return CheckProto_1(input, silent, value);
}

auto CheckProto_20(const Purse& input, const bool silent, std::string& value)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
