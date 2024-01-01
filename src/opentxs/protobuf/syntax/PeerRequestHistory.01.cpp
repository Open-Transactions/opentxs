// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/PeerRequestHistory.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/PeerEnums.pb.h>
#include <opentxs/protobuf/PeerRequestHistory.pb.h>
#include <string>

#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/PeerRequestWorkflow.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyContracts.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const PeerRequestHistory& input, const Log& log) -> bool
{
    switch (input.type()) {
        case PEERREQUEST_BAILMENT:
        case PEERREQUEST_OUTBAILMENT:
        case PEERREQUEST_PENDINGBAILMENT:
        case PEERREQUEST_CONNECTIONINFO:
        case PEERREQUEST_STORESECRET:
        case PEERREQUEST_VERIFIEDCLAIM:
        case PEERREQUEST_FAUCET: {
        } break;
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("Unsupported type.");
        }
    }

    CHECK_SUBOBJECTS(workflow, PeerRequestHistoryAllowedPeerRequestWorkflow());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
