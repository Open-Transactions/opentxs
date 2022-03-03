// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/p2p/Block.hpp"
// IWYU pragma: no_include "opentxs/network/p2p/State.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/util/Container.hpp"

namespace opentxs::network::p2p
{
using TypeEnum = std::uint32_t;

enum class MessageType : TypeEnum;

class Block;
class State;

using StateData = UnallocatedVector<p2p::State>;
using SyncData = UnallocatedVector<p2p::Block>;
}  // namespace opentxs::network::p2p

namespace opentxs
{
OPENTXS_EXPORT auto print(network::p2p::MessageType in) noexcept
    -> UnallocatedCString;

constexpr auto value(network::p2p::MessageType type) noexcept
{
    return static_cast<network::p2p::TypeEnum>(type);
}
}  // namespace opentxs
