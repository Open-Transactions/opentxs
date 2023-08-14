// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::blockchain::cfilter
{
using Targets = Vector<ReadView>;
using Matches = Vector<Targets::const_iterator>;
using TypeEnum = std::uint32_t;

// IWYU pragma: begin_exports
enum class Type : TypeEnum;  // IWYU pragma: keep
// IWYU pragma: end_exports

OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
}  // namespace opentxs::blockchain::cfilter
