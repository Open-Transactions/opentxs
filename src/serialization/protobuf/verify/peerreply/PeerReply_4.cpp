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
#include "internal/serialization/protobuf/verify/FaucetReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/NoticeAcknowledgement.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/OutBailmentReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerificationReply.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_4(const PeerReply& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(id);
    CHECK_IDENTIFIER(initiator);
    CHECK_IDENTIFIER(recipient);
    CHECK_EXISTS(type);
    CHECK_EXISTS(cookie);
    CHECK_SUBOBJECT(signature, PeerReplyAllowedSignature());

    switch (input.type()) {
        case PEERREQUEST_BAILMENT: {
            OPTIONAL_IDENTIFIER(server);
            CHECK_SUBOBJECT(bailment, PeerReplyAllowedBailment());
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            OPTIONAL_IDENTIFIER(server);
            CHECK_SUBOBJECT(outbailment, PeerReplyAllowedOutBailment());
        } break;
        case PEERREQUEST_PENDINGBAILMENT:
        case PEERREQUEST_STORESECRET:
        case PEERREQUEST_VERIFIEDCLAIM: {
            OPTIONAL_IDENTIFIER(server);
            CHECK_SUBOBJECT(notice, PeerReplyAllowedNotice());
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            OPTIONAL_IDENTIFIER(server);
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

auto CheckProto_5(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const PeerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
