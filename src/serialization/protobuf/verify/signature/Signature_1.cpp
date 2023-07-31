// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <Identifier.pb.h>
#include <Signature.pb.h>
#include <cstdint>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyContracts.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(
    const Signature& input,
    const bool silent,
    const proto::Identifier& selfID,
    const proto::Identifier& masterID,
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
        case SIGROLE_PEERREPLY: {
        } break;
        case SIGROLE_CONTEXT:
        case SIGROLE_ACCOUNT:
        case SIGROLE_SERVERREQUEST:
        case SIGROLE_SERVERREPLY:
        case SIGROLE_ERROR:
        case SIGROLE_NYM:
        default: {
            FAIL_2("invalid role", input.role());
        }
    }

    if ((SIGROLE_ERROR != role) && (role != input.role())) {
        FAIL_4("incorrect role", input.role(), " specified ", role);
    }

    if (proto::SIGROLE_NYMIDSOURCE != input.role()) {
        CHECK_SUBOBJECT(credentialid, SignatureAllowedIdentifier());
    }

    CHECK_EXISTS(hashtype);

    if (input.hashtype() > proto::HASHTYPE_BLAKE2B512) {
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

auto CheckProto_1(
    const Signature& input,
    const bool silent,
    const SignatureRole role) -> bool
{
    std::uint32_t unused = 0;
    auto blank = proto::Identifier{};

    return CheckProto_1(
        input, silent, blank, blank, unused, unused, unused, unused, role);
}
}  // namespace opentxs::proto
