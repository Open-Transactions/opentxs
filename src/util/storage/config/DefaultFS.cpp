// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "util/storage/Config.hpp"  // IWYU pragma: associated

namespace opentxs::storage
{
const UnallocatedCString Config::default_plugin_{OT_STORAGE_PRIMARY_PLUGIN_FS};
}  // namespace opentxs::storage
