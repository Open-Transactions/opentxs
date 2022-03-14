// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs::blockchain::cfilter
{
using TypeEnum = std::uint32_t;

enum class Type : TypeEnum;

using Hash = blockchain::Hash;
using pHash = blockchain::pHash;
using Header = Hash;
using pHeader = pHash;
}  // namespace opentxs::blockchain::cfilter
