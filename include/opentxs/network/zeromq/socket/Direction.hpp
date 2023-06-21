// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

namespace opentxs::network::zeromq::socket
{
enum class Direction : std::uint8_t {
    Bind = 0,
    Connect = 1,
};
}  // namespace opentxs::network::zeromq::socket
