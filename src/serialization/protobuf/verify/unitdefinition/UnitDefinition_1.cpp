// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"  // IWYU pragma: associated

#include <ContractEnums.pb.h>
#include <Enums.pb.h>
#include <Identifier.pb.h>
#include <Nym.pb.h>
#include <UnitDefinition.pb.h>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/BasketParams.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/CurrencyParams.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/EquityParams.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Nym.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContracts.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(
    const UnitDefinition& input,
    const bool silent,
    const bool checkSig) -> bool
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
}  // namespace opentxs::proto
