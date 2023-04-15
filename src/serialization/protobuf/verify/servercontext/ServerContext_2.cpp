// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/ServerContext.hpp"  // IWYU pragma: associated

#include <ConsensusEnums.pb.h>
#include <ServerContext.pb.h>
#include <functional>

#include "internal/serialization/protobuf/verify/PendingCommand.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyConsensus.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_2(const ServerContext& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(serverid);
    CHECK_EXCLUDED(state);
    CHECK_EXCLUDED(laststatus);
    CHECK_EXCLUDED(pending);

    return true;
}

auto CheckProto_3(const ServerContext& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(serverid);
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

auto CheckProto_4(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const ServerContext& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
