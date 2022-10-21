// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "core/FixedByteArray.tpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
#ifndef _MSC_VER
template class FixedByteArray<2_uz * sizeof(std::uint64_t)>;  // 16
template class FixedByteArray<3_uz * sizeof(std::uint64_t)>;  // 24
template class FixedByteArray<4_uz * sizeof(std::uint64_t)>;  // 32
#endif
}  // namespace opentxs
