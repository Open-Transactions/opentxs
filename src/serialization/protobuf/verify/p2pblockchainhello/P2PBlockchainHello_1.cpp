// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/serialization/protobuf/verify/P2PBlockchainHello.hpp"  // IWYU pragma: associated

#include <P2PBlockchainChainState.pb.h>
#include <P2PBlockchainHello.pb.h>  // IWYU pragma: keep
#include <cstddef>
#include <cstdint>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/P2PBlockchainChainState.hpp"  // IWYU pragma: keep
#include "internal/serialization/protobuf/verify/VerifyP2P.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/verify/Check.hpp"

namespace opentxs::proto
{
auto CheckProto_1(const P2PBlockchainHello& input, const bool silent) -> bool
{
    OPTIONAL_SUBOBJECTS(
        state, P2PBlockchainHelloAllowedP2PBlockchainChainState());

    auto map = UnallocatedMap<std::uint32_t, std::size_t>{};

    for (const auto& state : input.state()) {
        const auto& count = ++map[state.chain()];

        if (1 != count) { FAIL_2("Duplicate chain state", state.chain()); }
    }

    return true;
}

auto CheckProto_2(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2);
}

auto CheckProto_3(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3);
}

auto CheckProto_4(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4);
}

auto CheckProto_5(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5);
}

auto CheckProto_6(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6);
}

auto CheckProto_7(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7);
}

auto CheckProto_8(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8);
}

auto CheckProto_9(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9);
}

auto CheckProto_10(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10);
}

auto CheckProto_11(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11);
}

auto CheckProto_12(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12);
}

auto CheckProto_13(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13);
}

auto CheckProto_14(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14);
}

auto CheckProto_15(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15);
}

auto CheckProto_16(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16);
}

auto CheckProto_17(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17);
}

auto CheckProto_18(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18);
}

auto CheckProto_19(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19);
}

auto CheckProto_20(const P2PBlockchainHello& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::proto
