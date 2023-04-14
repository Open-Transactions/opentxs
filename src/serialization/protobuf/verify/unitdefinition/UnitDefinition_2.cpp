// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"  // IWYU pragma: associated

#include <BasketParams.pb.h>  // IWYU pragma: keep
#include <ContractEnums.pb.h>
#include <CurrencyParams.pb.h>  // IWYU pragma: keep
#include <Enums.pb.h>
#include <EquityParams.pb.h>  // IWYU pragma: keep
#include <Nym.pb.h>
#include <Signature.pb.h>  // IWYU pragma: keep
#include <UnitDefinition.pb.h>
#include <stdexcept>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/BasketParams.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/CurrencyParams.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/EquityParams.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Nym.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContracts.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_2(
    const UnitDefinition& input,
    const bool silent,
    const bool checkSig) -> bool
{
    if (!input.has_id()) { FAIL_1("missing id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
        FAIL_2("invalid id", input.id());
    }

    if (!input.has_issuer()) { FAIL_1("missing issuer id"); }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.issuer().size()) {
        FAIL_2("invalid issuer id", input.issuer());
    }

    if (!input.has_terms()) { FAIL_1("missing terms"); }

    if (1 > input.terms().size()) { FAIL_2("invalid terms", input.terms()); }

    if (!input.has_name()) { FAIL_1("missing name"); }

    if (1 > input.name().size()) { FAIL_2("invalid name", input.name()); }

    if (!input.has_type()) { FAIL_1("missing type"); }

    bool goodParams = false;

    switch (input.type()) {
        case UNITTYPE_CURRENCY: {
            if (!input.has_params()) { FAIL_1("missing currency params"); }

            try {
                goodParams = Check(
                    input.params(),
                    UnitDefinitionAllowedCurrencyParams()
                        .at(input.version())
                        .first,
                    UnitDefinitionAllowedCurrencyParams()
                        .at(input.version())
                        .second,
                    silent);
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed currency params version not defined for version",
                    input.version());
            }

            if (1 > input.params().scales().size()) { goodParams = false; }

            if (!goodParams) { FAIL_1("invalid currency params"); }
        } break;
        case UNITTYPE_SECURITY: {
            if (!input.has_security()) { FAIL_1("missing security params"); }

            try {
                goodParams = Check(
                    input.security(),
                    UnitDefinitionAllowedSecurityParams()
                        .at(input.version())
                        .first,
                    UnitDefinitionAllowedSecurityParams()
                        .at(input.version())
                        .second,
                    silent);
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed security params version not defined for version",
                    input.version());
            }

            if (!goodParams) { FAIL_1("invalid security params"); }
        } break;
        case UNITTYPE_BASKET: {
            if (!input.has_basket()) { FAIL_1("missing currency params"); }

            try {
                goodParams = Check(
                    input.basket(),
                    UnitDefinitionAllowedBasketParams()
                        .at(input.version())
                        .first,
                    UnitDefinitionAllowedBasketParams()
                        .at(input.version())
                        .second,
                    silent);
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed basket params version not defined for version",
                    input.version());
            }

            if (!goodParams) { FAIL_1("invalid basket params"); }
        } break;
        case UNITTYPE_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    if (input.has_issuer_nym()) {
        try {
            const bool goodPublicNym = Check(
                input.issuer_nym(),
                UnitDefinitionAllowedNym().at(input.version()).first,
                UnitDefinitionAllowedNym().at(input.version()).second,
                silent);

            if (!goodPublicNym) { FAIL_1("invalid nym"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed credential index version not defined for version",
                input.version());
        }

        if (input.issuer() != input.issuer_nym().nymid()) {
            FAIL_1("wrong nym");
        }
    }

    if (checkSig) {
        try {
            const bool valid = Check(
                input.signature(),
                UnitDefinitionAllowedSignature().at(input.version()).first,
                UnitDefinitionAllowedSignature().at(input.version()).second,
                silent,
                SIGROLE_UNITDEFINITION);

            if (false == valid) { FAIL_1("invalid signature"); }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed signature version not defined for version",
                input.version());
        }
    }

    return true;
}

auto CheckProto_3(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const UnitDefinition& input, const bool silent, const bool)
    -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
