// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Purse.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/CashEnums.pb.h>
#include <opentxs/protobuf/Purse.pb.h>
#include <optional>

#include "opentxs/Time.hpp"
#include "opentxs/protobuf/syntax/Envelope.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/SymmetricKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Token.hpp"         // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCash.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
using namespace std::literals;

auto version_1(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_1(const Purse& input, const Log& log, UnallocatedCString& value)
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

    auto allowedStates = UnallocatedSet<TokenState>{};
    auto validFrom = seconds_since_epoch(Time::min()).value();
    auto validTo = seconds_since_epoch(Time::max()).value();

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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
