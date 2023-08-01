// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/serialization/protobuf/Basic.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::proto
{
auto ContextAllowedClient() noexcept -> const VersionMap&;
auto ContextAllowedIdentifier() noexcept -> const VersionMap&;
auto ContextAllowedServer() noexcept -> const VersionMap&;
auto ContextAllowedSignature() noexcept -> const VersionMap&;
auto ServerContextAllowedIdentifier() noexcept -> const VersionMap&;
auto ServerContextAllowedPendingCommand() noexcept -> const VersionMap&;
auto ServerContextAllowedState() noexcept
    -> const UnallocatedMap<std::uint32_t, UnallocatedSet<int>>&;
auto ServerContextAllowedStatus() noexcept
    -> const UnallocatedMap<std::uint32_t, UnallocatedSet<int>>&;
}  // namespace opentxs::proto
