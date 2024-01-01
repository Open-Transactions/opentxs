// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Authority.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Authority.pb.h>
#include <opentxs/protobuf/Credential.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Identifier.pb.h>
#include <string>
#include <utility>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Credential.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/protobuf/syntax/VerifyCredentials.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    CHECK_SUBOBJECT(nymid, AuthorityAllowedIdentifier());

    if (input.nymid() != nymID) { FAIL_1("wrong nym id"); }

    CHECK_SUBOBJECT(masterid, AuthorityAllowedIdentifier());
    CHECK_EXISTS(mode);

    const auto checkMode = (AUTHORITYMODE_ERROR != mode);

    if (checkMode) {
        if (input.mode() != mode) { FAIL_2("incorrect mode", input.mode()); }
    }

    switch (input.mode()) {
        case AUTHORITYMODE_INDEX: {
            if (KEYMODE_PRIVATE == key) {
                if (1 > input.index()) { FAIL_1("missing index"); }
            } else {
                if (0 < input.index()) {
                    FAIL_1("index present in public mode");
                }
            }

            CHECK_EXCLUDED(mastercredential);
            CHECK_NONE(activechildren);
            CHECK_NONE(revokedchildren);
            CHECK_SUBOBJECTS(activechildids, AuthorityAllowedIdentifier());
            CHECK_SUBOBJECTS(revokedchildids, AuthorityAllowedIdentifier());
        } break;
        case AUTHORITYMODE_FULL: {
            CHECK_SUBOBJECT_VA(
                mastercredential,
                AuthorityAllowedCredential(),
                key,
                CREDROLE_MASTERKEY,
                true);

            if (CREDTYPE_HD == input.mastercredential().type()) {
                haveHD = true;
            }

            if (input.mastercredential().id() != input.masterid()) {
                FAIL_1("wrong master credential");
            }

            CHECK_NONE(activechildids);
            CHECK_NONE(revokedchildids);

            for (const auto& it : input.activechildren()) {
                if (!check_version(
                        it,
                        log,
                        AuthorityAllowedCredential().at(input.version()).first,
                        AuthorityAllowedCredential().at(input.version()).second,
                        key,
                        CREDROLE_ERROR,
                        true)) {
                    FAIL_1("invalid active child credential");
                }

                if (CREDTYPE_HD == it.type()) { haveHD = true; }

                if (CREDROLE_MASTERKEY == it.role()) {
                    FAIL_1("unexpected master credential");
                }
            }

            for (const auto& it : input.revokedchildren()) {
                if (!check_version(
                        it,
                        log,
                        AuthorityAllowedCredential().at(input.version()).first,
                        AuthorityAllowedCredential().at(input.version()).second,
                        key,
                        CREDROLE_ERROR,
                        true)) {
                    FAIL_1("invalid revoked child credential");
                }

                if (CREDTYPE_HD == it.type()) { haveHD = true; }

                if (CREDROLE_MASTERKEY == it.role()) {
                    FAIL_1("unexpected master credential");
                }
            }

            if (KEYMODE_PRIVATE == key) {
                FAIL_1("private credentials serialized in public form");
            } else {
                if (haveHD) {
                    if (0 < input.index()) {
                        FAIL_1("index present in public mode");
                    }
                }
            }

        } break;
        case AUTHORITYMODE_ERROR:
        default:
            FAIL_2("unknown mode", input.mode());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
