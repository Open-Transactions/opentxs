// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PeerRequest.hpp"  // IWYU pragma: associated

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/Bailment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ConnectionInfo.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Faucet.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/OutBailment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PendingBailment.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/StoreSecret.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationOffer.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/Enums.pb.h"
#include "serialization/protobuf/PeerEnums.pb.h"
#include "serialization/protobuf/PeerRequest.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_4(const PeerRequest& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(id)
    CHECK_IDENTIFIER(initiator)
    CHECK_IDENTIFIER(recipient)
    CHECK_EXISTS(type)
    CHECK_IDENTIFIER(cookie)
    CHECK_SUBOBJECT_VA(
        signature, PeerRequestAllowedSignature(), SIGROLE_PEERREQUEST)
    CHECK_IDENTIFIER(server)

    switch (input.type()) {
        case PEERREQUEST_BAILMENT: {
            CHECK_EXCLUDED(outbailment)
            CHECK_EXCLUDED(pendingbailment)
            CHECK_EXCLUDED(connectioninfo)
            CHECK_EXCLUDED(storesecret)
            CHECK_EXCLUDED(verificationoffer)
            CHECK_EXCLUDED(faucet)
            CHECK_SUBOBJECT(bailment, PeerRequestAllowedBailment())
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            CHECK_EXCLUDED(bailment)
            CHECK_EXCLUDED(pendingbailment)
            CHECK_EXCLUDED(connectioninfo)
            CHECK_EXCLUDED(storesecret)
            CHECK_EXCLUDED(verificationoffer)
            CHECK_EXCLUDED(faucet)
            CHECK_SUBOBJECT(outbailment, PeerRequestAllowedOutBailment())
        } break;
        case PEERREQUEST_PENDINGBAILMENT: {
            CHECK_EXCLUDED(bailment)
            CHECK_EXCLUDED(outbailment)
            CHECK_EXCLUDED(connectioninfo)
            CHECK_EXCLUDED(storesecret)
            CHECK_EXCLUDED(verificationoffer)
            CHECK_EXCLUDED(faucet)
            CHECK_SUBOBJECT(
                pendingbailment, PeerRequestAllowedPendingBailment())
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            CHECK_EXCLUDED(bailment)
            CHECK_EXCLUDED(outbailment)
            CHECK_EXCLUDED(pendingbailment)
            CHECK_EXCLUDED(storesecret)
            CHECK_EXCLUDED(verificationoffer)
            CHECK_EXCLUDED(faucet)
            CHECK_SUBOBJECT(connectioninfo, PeerRequestAllowedConnectionInfo())
        } break;
        case PEERREQUEST_STORESECRET: {
            CHECK_EXCLUDED(bailment)
            CHECK_EXCLUDED(outbailment)
            CHECK_EXCLUDED(pendingbailment)
            CHECK_EXCLUDED(connectioninfo)
            CHECK_EXCLUDED(verificationoffer)
            CHECK_EXCLUDED(faucet)
            CHECK_SUBOBJECT(storesecret, PeerRequestAllowedStoreSecret())
        } break;
        case PEERREQUEST_VERIFICATIONOFFER: {
            CHECK_EXCLUDED(bailment)
            CHECK_EXCLUDED(outbailment)
            CHECK_EXCLUDED(pendingbailment)
            CHECK_EXCLUDED(connectioninfo)
            CHECK_EXCLUDED(storesecret)
            CHECK_EXCLUDED(faucet)
            CHECK_SUBOBJECT(
                verificationoffer, PeerRequestAllowedVerificationOffer())
        } break;
        case PEERREQUEST_FAUCET: {
            CHECK_EXCLUDED(bailment)
            CHECK_EXCLUDED(outbailment)
            CHECK_EXCLUDED(pendingbailment)
            CHECK_EXCLUDED(connectioninfo)
            CHECK_EXCLUDED(storesecret)
            CHECK_EXCLUDED(verificationoffer)
            CHECK_SUBOBJECT(faucet, PeerRequestAllowedFaucet())
        } break;
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("invalid type")
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
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const PeerRequest& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
