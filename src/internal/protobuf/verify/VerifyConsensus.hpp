// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <set>

#include "internal/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
auto ContextAllowedServer() noexcept -> const VersionMap&;
auto ContextAllowedClient() noexcept -> const VersionMap&;
auto ContextAllowedSignature() noexcept -> const VersionMap&;
auto ServerContextAllowedPendingCommand() noexcept -> const VersionMap&;
auto ServerContextAllowedState() noexcept
    -> const std::pmr::map<std::uint32_t, std::pmr::set<int>>&;
auto ServerContextAllowedStatus() noexcept
    -> const std::pmr::map<std::uint32_t, std::pmr::set<int>>&;
}  // namespace proto
}  // namespace opentxs
