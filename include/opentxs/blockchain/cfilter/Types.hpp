// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::cfilter
{
using Targets = Vector<ReadView>;
using Matches = Vector<Targets::const_iterator>;

enum class Type : std::uint32_t;  // IWYU pragma: export

OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::cfilter
