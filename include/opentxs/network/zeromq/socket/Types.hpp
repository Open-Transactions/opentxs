// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::network::zeromq::socket
{
enum class Direction : bool { Bind = false, Connect = true };
enum class Type : std::uint8_t;  // IWYU pragma: export
}  // namespace opentxs::network::zeromq::socket

namespace opentxs
{
auto print(const network::zeromq::socket::Type) noexcept -> const char*;
auto to_native(const network::zeromq::socket::Type) noexcept -> int;
}  // namespace opentxs
