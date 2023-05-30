// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <span>
#include <string_view>

namespace ottest
{
OPENTXS_EXPORT auto HexWithoutPrefix() noexcept
    -> std::span<const std::string_view>;
OPENTXS_EXPORT auto HexWithPrefix() noexcept
    -> std::span<const std::string_view>;
}  // namespace ottest
