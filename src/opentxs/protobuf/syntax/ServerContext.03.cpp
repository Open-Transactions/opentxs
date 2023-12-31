// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/ServerContext.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ConsensusEnums.pb.h>
#include <opentxs/protobuf/ServerContext.pb.h>
#include <functional>

#include "opentxs/protobuf/syntax/Identifier.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/PendingCommand.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyConsensus.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_3(const ServerContext& input, const Log& log) -> bool
{
    CHECK_SUBOBJECT(serverid, ServerContextAllowedIdentifier());
    CHECK_MEMBERSHIP(state, ServerContextAllowedState());

    switch (input.state()) {
        case DELIVERTYSTATE_PENDINGSEND: {
            CHECK_SUBOBJECT(pending, ServerContextAllowedPendingCommand());
        } break;
        case DELIVERTYSTATE_NEEDNYMBOX:
        case DELIVERTYSTATE_NEEDBOXITEMS:
        case DELIVERTYSTATE_NEEDPROCESSNYMBOX: {
            OPTIONAL_SUBOBJECT(pending, ServerContextAllowedPendingCommand());
        } break;
        case DELIVERTYSTATE_IDLE:
        case DELIVERTYSTATE_ERROR:
        default: {
            CHECK_EXCLUDED(pending);
        }
    }

    CHECK_MEMBERSHIP(laststatus, ServerContextAllowedStatus());

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
