// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/interface/rpc/Types.hpp"  // IWYU pragma: keep

namespace opentxs::rpc
{
enum class PushType : std::underlying_type_t<PushType> {
    error = 0,
    account = 1,
    contact = 2,
    task = 3,
};
}  // namespace opentxs::rpc
