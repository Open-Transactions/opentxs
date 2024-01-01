// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PeerReply.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BailmentReply.pb.h>          // IWYU pragma: keep
#include <opentxs/protobuf/ConnectionInfoReply.pb.h>    // IWYU pragma: keep
#include <opentxs/protobuf/NoticeAcknowledgement.pb.h>  // IWYU pragma: keep
#include <opentxs/protobuf/OutBailmentReply.pb.h>       // IWYU pragma: keep
#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/PeerReply.pb.h>
#include <opentxs/protobuf/Signature.pb.h>  // IWYU pragma: keep
#include <string>

#include "opentxs/protobuf/syntax/BailmentReply.hpp"        // IWYU pragma: keep
#include "opentxs/protobuf/syntax/ConnectionInfoReply.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/FaucetReply.hpp"          // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Identifier.hpp"           // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/NoticeAcknowledgement.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/OutBailmentReply.hpp"   // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Signature.hpp"          // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerificationReply.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyPeer.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_4(const PeerReply& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(id, PeerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(initiator, PeerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(recipient, PeerReplyAllowedIdentifier());
    CHECK_EXISTS(type);
    CHECK_EXISTS(cookie);
    CHECK_SUBOBJECT(signature, PeerReplyAllowedSignature());

    switch (input.type()) {
        case PEERREQUEST_BAILMENT: {
            OPTIONAL_SUBOBJECT(server, PeerReplyAllowedIdentifier());
            CHECK_SUBOBJECT(bailment, PeerReplyAllowedBailment());
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            OPTIONAL_SUBOBJECT(server, PeerReplyAllowedIdentifier());
            CHECK_SUBOBJECT(outbailment, PeerReplyAllowedOutBailment());
        } break;
        case PEERREQUEST_PENDINGBAILMENT:
        case PEERREQUEST_STORESECRET:
        case PEERREQUEST_VERIFIEDCLAIM: {
            OPTIONAL_SUBOBJECT(server, PeerReplyAllowedIdentifier());
            CHECK_SUBOBJECT(notice, PeerReplyAllowedNotice());
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            OPTIONAL_SUBOBJECT(server, PeerReplyAllowedIdentifier());
            CHECK_SUBOBJECT(connectioninfo, PeerReplyAllowedConnectionInfo());
        } break;
        case PEERREQUEST_FAUCET: {
            CHECK_EXCLUDED(server);
            CHECK_SUBOBJECT(faucet, PeerReplyAllowedFaucetReply());
        } break;
        case PEERREQUEST_VERIFICATION: {
            CHECK_EXCLUDED(server);
            CHECK_SUBOBJECT(verification, PeerReplyAllowedVerificationReply());
        } break;
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
