// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/common/Blocks.hpp"  // IWYU pragma: associated
#include "blockchain/database/common/Sync.hpp"    // IWYU pragma: associated

namespace opentxs::blockchain::database::common
{
struct Blocks::Imp {
};

Blocks::Blocks(storage::lmdb::Database&, Bulk&) noexcept
    : imp_(std::make_unique<Imp>())
{
}

auto Blocks::Exists(const Hash&) const noexcept -> bool { return {}; }

auto Blocks::Load(const Hash&) const noexcept -> BlockReader { return {}; }

auto Blocks::Store(const Hash&, const std::size_t) const noexcept -> BlockWriter
{
    return {};
}

Blocks::~Blocks() = default;
}  // namespace opentxs::blockchain::database::common

namespace opentxs::blockchain::database::common
{
struct Sync::Imp {
};

Sync::Sync(
    const api::Session&,
    storage::lmdb::Database&,
    const std::filesystem::path&) noexcept(false)
    : imp_(std::make_unique<Imp>())
{
}

auto Sync::Load(const Chain, const Height, Message&) const noexcept -> bool
{
    return {};
}

auto Sync::Reorg(const Chain, const Height) const noexcept -> bool
{
    return {};
}

auto Sync::Store(const Chain, const network::otdht::SyncData&) const noexcept
    -> bool
{
    return {};
}

auto Sync::Tip(const Chain) const noexcept -> Height { return -1; }

Sync::~Sync() = default;
}  // namespace opentxs::blockchain::database::common
