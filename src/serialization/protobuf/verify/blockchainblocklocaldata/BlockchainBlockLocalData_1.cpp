// Copyright (c) 2018-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlockchainBlockLocalData.hpp"  // IWYU pragma: associated

#include "serialization/protobuf/BlockchainBlockLocalData.pb.h"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    if (input.has_work()) { CHECK_STRING_(work, 0, MAX_PLAUSIBLE_WORK); }
    if (input.has_inherit_work()) {
        CHECK_STRING_(inherit_work, 0, MAX_PLAUSIBLE_WORK);
    }

    return true;
}

auto CheckProto_2(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const BlockchainBlockLocalData& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
