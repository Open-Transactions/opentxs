// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Signature.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/Identifier.pb.h>
#include <opentxs/protobuf/Signature.pb.h>
#include <cstdint>
#include <string>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Constants.hpp"
#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_3(
    const Signature& input,
    const Log& log,
    const protobuf::Identifier& selfID,
    const protobuf::Identifier& masterID,
    std::uint32_t& selfPublic,
    std::uint32_t& selfPrivate,
    std::uint32_t& masterPublic,
    std::uint32_t& sourcePublic,
    const SignatureRole role) -> bool
{
    CHECK_EXISTS(role);

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

    if (protobuf::SIGROLE_NYMIDSOURCE != input.role()) {
        CHECK_SUBOBJECT(credentialid, SignatureAllowedIdentifier());
    }

    CHECK_EXISTS(hashtype);

    if (input.hashtype() > protobuf::HASHTYPE_ETHEREUM) {
        FAIL_2("invalid hash type", input.hashtype());
    }

    CHECK_EXISTS(signature);

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

auto version_3(const Signature& input, const Log& log, const SignatureRole role)
    -> bool
{
    std::uint32_t unused = 0;
    auto blank = protobuf::Identifier{};

    return version_3(
        input, log, blank, blank, unused, unused, unused, unused, role);
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
