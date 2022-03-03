// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/ChildCredentialParameters.hpp"  // IWYU pragma: associated

#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/ChildCredentialParameters.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    if (!input.has_masterid()) { FAIL_1("missing master identifier") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.masterid().size()) {
        FAIL_2("invalid master identifier", input.masterid())
    }

    return true;
}

auto CheckProto_2(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const ChildCredentialParameters& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
