// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/SourceProof.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/SourceProof.pb.h>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const SourceProof& input,
    const Log& log,
    bool& ExpectSourceSignature) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type"); }

    switch (input.type()) {
        case SOURCEPROOFTYPE_SELF_SIGNATURE: {
            ExpectSourceSignature = false;
        } break;
        case SOURCEPROOFTYPE_SIGNATURE: {
            ExpectSourceSignature = true;
        } break;
        case SOURCEPROOFTYPE_ERROR:
        default:
            FAIL_2("incorrect or unknown type", input.type());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
