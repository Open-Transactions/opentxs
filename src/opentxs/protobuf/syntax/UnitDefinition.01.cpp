// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/UnitDefinition.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContractEnums.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Identifier.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <opentxs/protobuf/UnitDefinition.pb.h>
#include <string>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/BasketParams.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/CurrencyParams.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/EquityParams.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Nym.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"

namespace opentxs::protobuf::inline syntax
{

auto version_1(const UnitDefinition& input, const Log& log, const bool checkSig)
    -> bool
{
    CHECK_SUBOBJECT(id, UnitDefinitionAllowedIdentifier());
    CHECK_SUBOBJECT(issuer, UnitDefinitionAllowedIdentifier());
    CHECK_EXISTS(terms);
    CHECK_EXISTS(name);
    CHECK_EXISTS(type);

    switch (input.type()) {
        case UNITTYPE_CURRENCY: {
            CHECK_SUBOBJECT(params, UnitDefinitionAllowedCurrencyParams());
        } break;
        case UNITTYPE_SECURITY: {
            CHECK_SUBOBJECT(security, UnitDefinitionAllowedSecurityParams());
        } break;
        case UNITTYPE_BASKET: {
            CHECK_SUBOBJECT(basket, UnitDefinitionAllowedBasketParams());
        } break;
        case UNITTYPE_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    OPTIONAL_SUBOBJECT(issuer_nym, UnitDefinitionAllowedNym());

    if (input.has_issuer_nym() && (input.issuer() != input.issuer_nym().id())) {
        FAIL_1("wrong nym");
    }

    if (checkSig) {
        CHECK_SUBOBJECT_VA(
            signature,
            UnitDefinitionAllowedSignature(),
            SIGROLE_UNITDEFINITION);
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
