// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::crypto::key
{
namespace asymmetric
{
enum class Algorithm : std::uint8_t;  // IWYU pragma: export
enum class Mode : std::uint8_t;       // IWYU pragma: export
enum class Role : std::uint8_t;       // IWYU pragma: export
}  // namespace asymmetric

namespace symmetric
{
enum class Source : std::uint8_t;     // IWYU pragma: export
enum class Algorithm : std::uint8_t;  // IWYU pragma: export
}  // namespace symmetric
}  // namespace opentxs::crypto::key
