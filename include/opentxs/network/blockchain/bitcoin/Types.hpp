// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::network::blockchain::bitcoin
{
// IWYU pragma: begin_exports
enum class Service : std::uint8_t;  // IWYU pragma: keep
// IWYU pragma: end_exports

OPENTXS_EXPORT auto print(Service) noexcept -> std::string_view;
}  // namespace opentxs::network::blockchain::bitcoin