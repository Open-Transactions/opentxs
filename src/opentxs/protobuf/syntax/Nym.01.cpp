// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Nym.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Nym.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Authority.hpp"   // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/NymIDSource.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const Nym& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(id, NymAllowedIdentifier());
    CHECK_EXISTS(mode);

    const auto actualMode = input.mode();

    CHECK_EXISTS(revision);

    if (1 > input.revision()) { FAIL_2("invalid revision", input.revision()); }

    CHECK_SUBOBJECT(source, NymAllowedNymIDSource());

    auto haveHD = false;
    const auto mode =
        (NYM_PRIVATE == actualMode) ? KEYMODE_PRIVATE : KEYMODE_PUBLIC;

    OPTIONAL_SUBOBJECTS_VA(
        activecredentials, NymAllowedAuthority(), input.id(), mode, haveHD);
    OPTIONAL_SUBOBJECTS_VA(
        revokedcredentials, NymAllowedAuthority(), input.id(), mode, haveHD);

    switch (actualMode) {
        case NYM_PRIVATE: {
            if (haveHD) {
                if (1 > input.index()) { FAIL_1("missing index"); }
            }

            break;
        }
        case NYM_PUBLIC: {
            if (input.has_index()) { FAIL_1("index present in public mode"); }

            break;
        }
        case NYM_ERROR:
        default: {
            FAIL_2("invalid mode", actualMode);
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
