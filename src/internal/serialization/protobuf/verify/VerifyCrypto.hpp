// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/serialization/protobuf/Basic.hpp"

namespace opentxs::proto
{
auto HDPathAllowedIdentifier() noexcept -> const VersionMap&;
auto HDSeedAllowedIdentifier() noexcept -> const VersionMap&;
}  // namespace opentxs::proto
