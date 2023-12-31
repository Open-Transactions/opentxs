// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PeerRequest.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/PeerRequest.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Bailment.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/ConnectionInfo.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/OutBailment.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/PendingBailment.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/StoreSecret.hpp"      // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyPeer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const PeerRequest& input, const Log& log) -> bool
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
            CHECK_EXCLUDED(outbailment);
            CHECK_EXCLUDED(pendingbailment);
            CHECK_EXCLUDED(connectioninfo);
            CHECK_EXCLUDED(storesecret);
            CHECK_SUBOBJECT(bailment, PeerRequestAllowedBailment());
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            CHECK_EXCLUDED(bailment);
            CHECK_EXCLUDED(pendingbailment);
            CHECK_EXCLUDED(connectioninfo);
            CHECK_EXCLUDED(storesecret);
            CHECK_SUBOBJECT(outbailment, PeerRequestAllowedOutBailment());
        } break;
        case PEERREQUEST_PENDINGBAILMENT: {
            CHECK_EXCLUDED(bailment);
            CHECK_EXCLUDED(outbailment);
            CHECK_EXCLUDED(connectioninfo);
            CHECK_EXCLUDED(storesecret);
            CHECK_SUBOBJECT(
                pendingbailment, PeerRequestAllowedPendingBailment());
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            CHECK_EXCLUDED(bailment);
            CHECK_EXCLUDED(outbailment);
            CHECK_EXCLUDED(pendingbailment);
            CHECK_EXCLUDED(storesecret);
            CHECK_SUBOBJECT(connectioninfo, PeerRequestAllowedConnectionInfo());
        } break;
        case PEERREQUEST_STORESECRET: {
            CHECK_EXCLUDED(bailment);
            CHECK_EXCLUDED(outbailment);
            CHECK_EXCLUDED(pendingbailment);
            CHECK_EXCLUDED(connectioninfo);
            CHECK_SUBOBJECT(storesecret, PeerRequestAllowedStoreSecret());
        } break;
        case PEERREQUEST_VERIFIEDCLAIM:
        case PEERREQUEST_FAUCET:
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    CHECK_EXCLUDED(verificationoffer);
    CHECK_EXCLUDED(faucet);

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
