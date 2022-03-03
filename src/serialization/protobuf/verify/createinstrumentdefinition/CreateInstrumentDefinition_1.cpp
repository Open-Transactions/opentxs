// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/CreateInstrumentDefinition.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/Contact.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/CreateInstrumentDefinition.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    CHECK_NAME(name);
    CHECK_NAME(symbol);
    CHECK_NAME(terms);

    const auto allowedtype = 1 == AllowedItemTypes()
                                      .at({5, CONTACTSECTION_CONTRACT})
                                      .count(input.unitofaccount());

    if (false == allowedtype) { FAIL_1("Invalid unit of account") }

    return true;
}

auto CheckProto_2(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const CreateInstrumentDefinition& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
