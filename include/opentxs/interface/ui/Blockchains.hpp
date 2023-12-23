// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/interface/ui/Types.hpp"  // IWYU pragma: keep

namespace opentxs::ui
{
enum class Blockchains : std::underlying_type_t<Blockchains> {
    All = 0,
    Main = 1,
    Test = 2,
};
}  // namespace opentxs::ui
