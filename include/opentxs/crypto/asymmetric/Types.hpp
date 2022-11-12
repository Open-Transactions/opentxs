// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::crypto::asymmetric
{
enum class Algorithm : std::uint8_t;
enum class Mode : std::uint8_t;
enum class Role : std::uint8_t;

OPENTXS_EXPORT auto print(Algorithm) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Mode) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Role) noexcept -> std::string_view;
}  // namespace opentxs::crypto::asymmetric
