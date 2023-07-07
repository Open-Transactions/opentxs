// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/core/contract/peer/Types.hpp"

namespace opentxs::contract::peer::internal
{
enum class PairEventType : std::uint32_t {
    Error = 0,
    Rename = 1,
    StoreSecret = 2,
};  // IWYU pragma: export
}  // namespace opentxs::contract::peer::internal
