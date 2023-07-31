// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PeerRequest.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <PeerEnums.pb.h>
#include <PeerRequest.pb.h>

#include "internal/serialization/protobuf/verify/Bailment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ConnectionInfo.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Faucet.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/OutBailment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PendingBailment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StoreSecret.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationOffer.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationRequest.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_4(const PeerRequest& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(id, PeerRequestAllowedIdentifier());
    CHECK_SUBOBJECT(initiator, PeerRequestAllowedIdentifier());
    CHECK_SUBOBJECT(recipient, PeerRequestAllowedIdentifier());
    CHECK_EXISTS(type);
    CHECK_SUBOBJECT(cookie, PeerRequestAllowedIdentifier());
    CHECK_SUBOBJECT_VA(
        signature, PeerRequestAllowedSignature(), SIGROLE_PEERREQUEST);

    switch (input.type()) {
        case PEERREQUEST_BAILMENT: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(bailment, PeerRequestAllowedBailment());
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(outbailment, PeerRequestAllowedOutBailment());
        } break;
        case PEERREQUEST_PENDINGBAILMENT: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(
                pendingbailment, PeerRequestAllowedPendingBailment());
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(connectioninfo, PeerRequestAllowedConnectionInfo());
        } break;
        case PEERREQUEST_STORESECRET: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(storesecret, PeerRequestAllowedStoreSecret());
        } break;
        case PEERREQUEST_VERIFIEDCLAIM: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(
                verificationoffer, PeerRequestAllowedVerificationOffer());
        } break;
        case PEERREQUEST_FAUCET: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(faucet, PeerRequestAllowedFaucet());
        } break;
        case PEERREQUEST_VERIFICATION: {
            OPTIONAL_SUBOBJECT(server, PeerRequestAllowedIdentifier());
            CHECK_SUBOBJECT(
                verification, PeerRequestAllowedVerificationRequest());
        } break;
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}

auto CheckProto_5(const PeerRequest& input, const bool silent) -> bool
{
    return CheckProto_4(input, silent);
}

auto CheckProto_6(const PeerRequest& input, const bool silent) -> bool
{
    return CheckProto_4(input, silent);
}

auto CheckProto_7(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
