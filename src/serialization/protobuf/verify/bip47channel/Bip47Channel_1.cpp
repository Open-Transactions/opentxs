// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/Bip47Channel.hpp"  // IWYU pragma: associated

#include <Bip47Channel.pb.h>
#include <ContactEnums.pb.h>  // IWYU pragma: keep

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/Bip47Direction.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/BlockchainActivity.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/BlockchainDeterministicAccountData.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/HDPath.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/PaymentCode.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyBlockchain.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"  // IWYU pragma: keep
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{

auto CheckProto_1(const Bip47Channel& input, const bool silent) -> bool
{
    CHECK_SUBOBJECT(
        deterministic, Bip47ChannelAllowedBlockchainDeterministicAccountData());
    CHECK_SUBOBJECT(local, Bip47ChannelAllowedPaymentCode());
    CHECK_SUBOBJECT(remote, Bip47ChannelAllowedPaymentCode());
    CHECK_SUBOBJECT(incoming, Bip47ChannelAllowedBip47Direction());
    CHECK_SUBOBJECT(outgoing, Bip47ChannelAllowedBip47Direction());
    OPTIONAL_IDENTIFIER(contact);

    return true;
}

auto CheckProto_2(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const Bip47Channel& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
