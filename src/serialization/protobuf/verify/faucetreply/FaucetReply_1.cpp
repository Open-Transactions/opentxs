// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/FaucetReply.hpp"  // IWYU pragma: associated

#include <FaucetReply.pb.h>

#include "internal/serialization/protobuf/verify/BlockchainTransaction.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyPeer.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const FaucetReply& input, const bool silent) -> bool
{
    OPTIONAL_IDENTIFIER(txid);
    OPTIONAL_SUBOBJECT(transaction, FaucetReplyAllowedBlockchainTransaction());

    return true;
}

auto CheckProto_2(const FaucetReply& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const FaucetReply& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(const FaucetReply& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_5(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const FaucetReply& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
