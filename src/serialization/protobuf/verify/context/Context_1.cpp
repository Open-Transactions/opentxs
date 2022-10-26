// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Context.hpp"  // IWYU pragma: associated

#include <ConsensusEnums.pb.h>
#include <Context.pb.h>
#include <Enums.pb.h>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/ClientContext.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/ServerContext.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyConsensus.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const Context& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(localnym);
    CHECK_IDENTIFIER(remotenym);
    CHECK_EXISTS(type);

    switch (input.type()) {
        case CONSENSUSTYPE_SERVER: {
            CHECK_EXCLUDED(clientcontext);
            CHECK_SUBOBJECT(servercontext, ContextAllowedServer());
        } break;
        case CONSENSUSTYPE_CLIENT: {
            CHECK_EXCLUDED(servercontext);
            CHECK_SUBOBJECT(clientcontext, ContextAllowedServer());
        } break;
        case CONSENSUSTYPE_PEER:
        case CONSENSUSTYPE_ERROR:
        default: {
            FAIL_1("invalid type");
        }
    }

    CHECK_SUBOBJECT_VA(signature, ContextAllowedSignature(), SIGROLE_CONTEXT);

    return true;
}

auto CheckProto_2(const Context& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const Context& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const Context& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
