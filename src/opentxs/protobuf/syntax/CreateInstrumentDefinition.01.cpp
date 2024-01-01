// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/CreateInstrumentDefinition.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_set.hpp>
#include <opentxs/protobuf/ContactSectionName.pb.h>
#include <opentxs/protobuf/CreateInstrumentDefinition.pb.h>
#include <utility>

#include "opentxs/protobuf/contact/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const CreateInstrumentDefinition& input, const Log& log) -> bool
{
    CHECK_NAME(name);
    CHECK_NAME(symbol);
    CHECK_NAME(terms);

    const auto allowedtype = 1 == contact::AllowedItemTypes()
                                      .at({5, CONTACTSECTION_CONTRACT})
                                      .count(input.unitofaccount());

    if (false == allowedtype) { FAIL_1("Invalid unit of account"); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
