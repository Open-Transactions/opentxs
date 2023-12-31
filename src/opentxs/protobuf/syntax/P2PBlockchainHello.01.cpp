// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/P2PBlockchainHello.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/P2PBlockchainChainState.pb.h>
#include <opentxs/protobuf/P2PBlockchainHello.pb.h>  // IWYU pragma: keep
#include <cstddef>
#include <cstdint>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/protobuf/syntax/P2PBlockchainChainState.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/VerifyP2P.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const P2PBlockchainHello& input, const Log& log) -> bool
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
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
