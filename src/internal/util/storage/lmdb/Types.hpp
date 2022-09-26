// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

extern "C" {
using MDB_dbi = unsigned int;
}

namespace opentxs::storage::lmdb
{
using Callback = std::function<void(const ReadView data)>;
using Flags = unsigned int;
using ReadCallback =
    std::function<bool(const ReadView key, const ReadView value)>;
using Result = std::pair<bool, int>;
using Table = int;
using Databases = UnallocatedMap<Table, MDB_dbi>;
using TablesToInit = UnallocatedVector<std::pair<Table, unsigned int>>;
using TableNames = UnallocatedMap<Table, const UnallocatedCString>;
using UpdateCallback = std::function<Space(const ReadView data)>;

enum class Dir : bool { Forward = false, Backward = true };
enum class Mode : bool { One = false, Multiple = true };
}  // namespace opentxs::storage::lmdb
