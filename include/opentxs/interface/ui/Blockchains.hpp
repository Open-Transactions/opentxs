// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/interface/ui/Types.hpp"

namespace opentxs::ui
{
enum class Blockchains : std::uint8_t {
    All = 0,
    Main = 1,
    Test = 2,
};
}  // namespace opentxs::ui
