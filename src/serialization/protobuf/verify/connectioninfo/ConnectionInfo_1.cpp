// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/ConnectionInfo.hpp"  // IWYU pragma: associated

#include <ConnectionInfo.pb.h>
#include <PeerEnums.pb.h>

#include "internal/serialization/protobuf/verify/Identifier.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const ConnectionInfo& input, const bool silent) -> bool
{
    CHECK_EXISTS(type);

    if ((CONNECTIONINFO_BITCOIN > input.type()) ||
        (CONNECTIONINFO_BITMESSAGERPC < input.type())) {
        FAIL_2("invalid type", input.type());
    }

    CHECK_EXCLUDED(nym);

    return true;
}

auto CheckProto_2(const ConnectionInfo& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const ConnectionInfo& input, const bool silent) -> bool
{
    CHECK_EXISTS(type);

    if ((CONNECTIONINFO_BITCOIN > input.type()) ||
        (CONNECTIONINFO_CJDNS < input.type())) {
        FAIL_2("invalid type", input.type());
    }

    OPTIONAL_SUBOBJECT(nym, ConnectionInfoAllowedIdentifier());

    return true;
}

auto CheckProto_4(const ConnectionInfo& input, const bool silent) -> bool
{
    return CheckProto_3(input, silent);
}

auto CheckProto_5(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const ConnectionInfo& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
