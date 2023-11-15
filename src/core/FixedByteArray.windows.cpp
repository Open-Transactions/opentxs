// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/FixedByteArray.tpp"  // IWYU pragma: associated

#include <cstdint>

#include "internal/util/P0330.hpp"

namespace opentxs
{
template class OPENTXS_EXPORT FixedByteArray<2_uz * sizeof(std::uint64_t)>;
template class OPENTXS_EXPORT FixedByteArray<3_uz * sizeof(std::uint64_t)>;
template class OPENTXS_EXPORT FixedByteArray<4_uz * sizeof(std::uint64_t)>;
}  // namespace opentxs
