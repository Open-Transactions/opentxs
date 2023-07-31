// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/ServerReply.hpp"  // IWYU pragma: associated

#include <Enums.pb.h>
#include <OTXEnums.pb.h>
#include <ServerReply.pb.h>

#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/OTXPush.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/Signature.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyOTX.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const ServerReply& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(id, ServerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(nym, ServerReplyAllowedIdentifier());
    CHECK_SUBOBJECT(server, ServerReplyAllowedIdentifier());
    CHECK_SUBOBJECT_VA(
        signature, ServerReplyAllowedSignature(), SIGROLE_SERVERREPLY);

    switch (input.type()) {
        case SERVERREPLY_ACTIVATE: {
            CHECK_EXCLUDED(push);
        } break;
        case SERVERREPLY_PUSH: {
            CHECK_SUBOBJECT(push, ServerReplyAllowedOTXPush());
        } break;
        case SERVERREPLY_ERROR:
        default: {
            FAIL_1("Invalid type");
        }
    }

    return true;
}

auto CheckProto_2(const ServerReply& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const ServerReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
