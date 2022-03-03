// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/BlockchainP2PHello.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>

#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/BlockchainP2PChainState.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyBlockchain.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/BlockchainP2PChainState.pb.h"
#include "serialization/protobuf/BlockchainP2PHello.pb.h"  // IWYU pragma: keep
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const BlockchainP2PHello& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECTS(
        state, BlockchainP2PHelloAllowedBlockchainP2PChainState())

    auto map = UnallocatedMap<std::uint32_t, std::size_t>{};

    for (const auto& state : input.state()) {
        const auto& count = ++map[state.chain()];

        if (1 != count) { FAIL_2("Duplicate chain state", state.chain()) }
    }

    return true;
}

auto CheckProto_2(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const BlockchainP2PHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
