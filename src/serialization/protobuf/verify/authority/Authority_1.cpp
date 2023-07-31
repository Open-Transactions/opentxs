// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Authority.hpp"  // IWYU pragma: associated

#include <Authority.pb.h>
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <Identifier.pb.h>
#include <utility>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Credential.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyCredentials.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const Authority& input,
    const bool silent,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    CHECK_SUBOBJECT(nymid, AuthorityAllowedIdentifier());

    if (input.nymid() != nymID) { FAIL_1("wrong nym id"); }

    CHECK_SUBOBJECT(masterid, AuthorityAllowedIdentifier());
    CHECK_EXISTS(mode);

    const bool checkMode = (AUTHORITYMODE_ERROR != mode);

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
                if (!Check(
                        it,
                        AuthorityAllowedCredential().at(input.version()).first,
                        AuthorityAllowedCredential().at(input.version()).second,
                        silent,
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
                if (!Check(
                        it,
                        AuthorityAllowedCredential().at(input.version()).first,
                        AuthorityAllowedCredential().at(input.version()).second,
                        silent,
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

auto CheckProto_2(
    const Authority& input,
    const bool silent,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    return CheckProto_1(input, silent, nymID, key, haveHD, mode);
}

auto CheckProto_3(
    const Authority& input,
    const bool silent,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    return CheckProto_1(input, silent, nymID, key, haveHD, mode);
}

auto CheckProto_4(
    const Authority& input,
    const bool silent,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    return CheckProto_1(input, silent, nymID, key, haveHD, mode);
}

auto CheckProto_5(
    const Authority& input,
    const bool silent,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    return CheckProto_1(input, silent, nymID, key, haveHD, mode);
}

auto CheckProto_6(
    const Authority& input,
    const bool silent,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    return CheckProto_1(input, silent, nymID, key, haveHD, mode);
}

auto CheckProto_7(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const Authority& input,
    const bool silent,
    const Identifier&,
    const KeyMode&,
    bool&,
    const AuthorityMode&) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
