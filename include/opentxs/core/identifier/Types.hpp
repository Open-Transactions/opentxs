// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

namespace opentxs::identifier
{
enum class Algorithm : std::uint8_t;
enum class Type : std::uint16_t;
}  // namespace opentxs::identifier

namespace opentxs
{
auto print(identifier::Algorithm) noexcept -> const char*;
auto print(identifier::Type) noexcept -> const char*;
}  // namespace opentxs