// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/network/zeromq/socket/Types.hpp"  // IWYU pragma: keep

namespace opentxs::network::zeromq::socket
{
enum class Type : std::underlying_type_t<Type> {
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
};  // IWYU pragma: export
}  // namespace opentxs::network::zeromq::socket
