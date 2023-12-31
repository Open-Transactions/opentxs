// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Token.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/CashEnums.pb.h>
#include <opentxs/protobuf/Token.pb.h>
#include <algorithm>
#include <cstdint>

#include "internal/core/Factory.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/protobuf/syntax/LucreTokenData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyCash.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Token& input,
    const Log& log,
    const CashType type,
    const UnallocatedSet<TokenState>& state,
    UnallocatedCString& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool
{
    if (type != input.type()) {
        FAIL_4(
            "Incorrect type",
            std::to_string(input.state()),
            " expected ",
            std::to_string(type));
    }

    const bool haveExpectedState = false == state.empty();

    if (haveExpectedState && (false == state.contains(input.state()))) {
        FAIL_2("Incorrect state", std::to_string(input.state()));
    }

    switch (input.state()) {
        case TOKENSTATE_BLINDED:
        case TOKENSTATE_SIGNED:
        case TOKENSTATE_READY: {
            const auto total = opentxs::factory::Amount(totalValue) +
                               opentxs::factory::Amount(input.denomination());
            if (false == total.Serialize(writer(totalValue))) {
                FAIL_2("Invalid value", totalValue);
            }
            validFrom = std::max(input.validfrom(), validFrom);
            validTo = std::min(input.validto(), validTo);
        } break;
        case TOKENSTATE_SPENT:
        case TOKENSTATE_EXPIRED: {
        } break;
        case TOKENSTATE_ERROR:
        default: {
            FAIL_2("Invalid state", std::to_string(input.state()));
        }
    }

    CHECK_IDENTIFIER(notary);
    CHECK_IDENTIFIER(mint);

    switch (input.type()) {
        case CASHTYPE_LUCRE: {
            CHECK_SUBOBJECT_VA(
                lucre, TokenAllowedLucreTokenData(), input.state());
        } break;
        case CASHTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", std::to_string(input.type()));
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
