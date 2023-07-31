// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/PeerReply.hpp"  // IWYU pragma: associated

#include <BailmentReply.pb.h>          // IWYU pragma: keep
#include <ConnectionInfoReply.pb.h>    // IWYU pragma: keep
#include <NoticeAcknowledgement.pb.h>  // IWYU pragma: keep
#include <OutBailmentReply.pb.h>       // IWYU pragma: keep
#include <PeerEnums.pb.h>
#include <PeerReply.pb.h>
#include <Signature.pb.h>  // IWYU pragma: keep

#include "internal/serialization/protobuf/verify/BailmentReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ConnectionInfoReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/NoticeAcknowledgement.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/OutBailmentReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_3(const PeerReply& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(id, PeerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(initiator, PeerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(recipient, PeerReplyAllowedIdentifier());
    CHECK_EXISTS(type);
    CHECK_EXISTS(cookie);
    CHECK_SUBOBJECT(signature, PeerReplyAllowedSignature());
    CHECK_SUBOBJECT(server, PeerReplyAllowedIdentifier());

    switch (input.type()) {
        case PEERREQUEST_BAILMENT: {
            CHECK_SUBOBJECT(bailment, PeerReplyAllowedBailment());
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            CHECK_SUBOBJECT(outbailment, PeerReplyAllowedOutBailment());
        } break;
        case PEERREQUEST_PENDINGBAILMENT:
        case PEERREQUEST_STORESECRET:
        case PEERREQUEST_VERIFIEDCLAIM: {
            CHECK_SUBOBJECT(notice, PeerReplyAllowedNotice());
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            CHECK_SUBOBJECT(connectioninfo, PeerReplyAllowedConnectionInfo());
        } break;
        case PEERREQUEST_FAUCET:
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    return true;
}
}  // namespace opentxs::proto
