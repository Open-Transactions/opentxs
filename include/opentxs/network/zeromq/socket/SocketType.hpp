// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::network::zeromq::socket
{
enum class Type : std::uint8_t {
    Error = 0,
    Request = 1,
    Reply = 2,
    Publish = 3,
    Subscribe = 4,
    Push = 5,
    Pull = 6,
    Pair = 7,
    Dealer = 8,
    Router = 9,
};
}  // namespace opentxs::network::zeromq::socket
