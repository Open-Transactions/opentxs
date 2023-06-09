// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <Signature.pb.h>
#include <cstdint>

#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_3(
    const Signature& input,
    const bool silent,
    const UnallocatedCString& selfID,
    const UnallocatedCString& masterID,
    std::uint32_t& selfPublic,
    std::uint32_t& selfPrivate,
    std::uint32_t& masterPublic,
    std::uint32_t& sourcePublic,
    const SignatureRole role) -> bool
{
    if (!input.has_role()) { FAIL_1("missing role"); }

    switch (input.role()) {
        case SIGROLE_PUBCREDENTIAL:
        case SIGROLE_PRIVCREDENTIAL:
        case SIGROLE_NYMIDSOURCE:
        case SIGROLE_CLAIM:
        case SIGROLE_SERVERCONTRACT:
        case SIGROLE_UNITDEFINITION:
        case SIGROLE_PEERREQUEST:
        case SIGROLE_PEERREPLY:
        case SIGROLE_CONTEXT:
        case SIGROLE_ACCOUNT:
        case SIGROLE_SERVERREQUEST:
        case SIGROLE_SERVERREPLY:
        case SIGROLE_NYM: {
        } break;
        case SIGROLE_ERROR:
        default: {
            FAIL_2("invalid role", input.role());
        }
    }

    if ((SIGROLE_ERROR != role) && (role != input.role())) {
        FAIL_4("incorrect role", input.role(), " specified ", role);
    }

    if (proto::SIGROLE_NYMIDSOURCE != input.role()) {

        if (!input.has_credentialid()) {
            FAIL_1(" missing credential identifier");
        }

        if (MIN_PLAUSIBLE_IDENTIFIER > input.credentialid().size()) {
            FAIL_2("invalid credential id", input.credentialid());
        }
    }

    if (!input.has_hashtype()) { FAIL_1("missing hashtype"); }

    if (input.hashtype() > proto::HASHTYPE_X11) {
        FAIL_2("invalid hash type", input.hashtype());
    }

    if (!input.has_signature()) { FAIL_1("missing signature"); }

    if (MIN_PLAUSIBLE_SIGNATURE > input.signature().size()) {
        FAIL_1("invalid signature");
    }

    if ((SIGROLE_PUBCREDENTIAL == input.role()) &&
        (selfID == input.credentialid())) {
        selfPublic += 1;
    }

    if ((SIGROLE_PUBCREDENTIAL == input.role()) &&
        (masterID == input.credentialid())) {
        masterPublic += 1;
    }

    if ((SIGROLE_PRIVCREDENTIAL == input.role()) &&
        (selfID == input.credentialid())) {
        selfPrivate += 1;
    }

    if (SIGROLE_NYMIDSOURCE == input.role()) { sourcePublic += 1; }

    return true;
}

auto CheckProto_3(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    std::uint32_t unused = 0;

    return CheckProto_3(
        input, silent, "", "", unused, unused, unused, unused, role);
}

auto CheckProto_4(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_4(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    std::uint32_t unused = 0;

    return CheckProto_4(
        input, silent, "", "", unused, unused, unused, unused, role);
}

auto CheckProto_5(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_5(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    std::uint32_t unused = 0;

    return CheckProto_5(
        input, silent, "", "", unused, unused, unused, unused, role);
}

auto CheckProto_6(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const Signature& input,
    const bool silent,
    const UnallocatedCString&,
    const UnallocatedCString&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    std::uint32_t&,
    const SignatureRole) -> bool
{
    UNDEFINED_VERSION(20);
}

auto CheckProto_6(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
