// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <filesystem>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block

namespace cfilter
{
class Header;
}  // namespace cfilter
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::params
{
auto WriteCheckpoints(const std::filesystem::path& outputDirectory) noexcept
    -> bool;
auto WriteCheckpoint(
    const std::filesystem::path& outputDirectory,
    blockchain::Type chain) noexcept -> bool;
auto WriteCheckpoint(
    const std::filesystem::path& outputDirectory,
    const block::Position& current,
    const block::Position& prior,
    const cfilter::Header& cfheader,
    blockchain::Type chain,
    cfilter::Type type) noexcept -> bool;
}  // namespace opentxs::blockchain::params
