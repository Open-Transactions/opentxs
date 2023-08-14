// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/bloom/Types.hpp"  // IWYU pragma: associated

#include "opentxs/Export.hpp"

namespace opentxs::blockchain::bloom
{
enum class UpdateFlag : std::uint8_t {
    None = 0,
    All = 1,
    PubkeyOnly = 2
};  // IWYU pragma: export
}  // namespace opentxs::blockchain::bloom
