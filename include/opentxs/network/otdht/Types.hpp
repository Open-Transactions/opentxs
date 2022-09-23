// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/otdht/Block.hpp"
// IWYU pragma: no_include "opentxs/network/otdht/State.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string_view>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace otdht
{
class Block;
class State;
}  // namespace otdht
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
using TypeEnum = std::uint32_t;

enum class MessageType : TypeEnum;  // IWYU pragma: export

using StateData = Vector<otdht::State>;
using SyncData = Vector<otdht::Block>;

OPENTXS_EXPORT auto print(MessageType in) noexcept -> std::string_view;

constexpr auto value(MessageType type) noexcept
{
    return static_cast<TypeEnum>(type);
}
}  // namespace opentxs::network::otdht
