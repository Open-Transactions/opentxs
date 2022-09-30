// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
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

auto Blocks::Exists(const block::Hash& block) const noexcept -> bool
{
    return {};
}

auto Blocks::Forget(const block::Hash& block) const noexcept -> bool
{
    return {};
}

auto Blocks::Load(const block::Hash& block) const noexcept -> ReadView
{
    return {};
}

auto Blocks::Store(const block::Block& block) const noexcept -> bool
{
    return {};
}

Blocks::~Blocks() = default;
}  // namespace opentxs::blockchain::database::common

namespace opentxs::blockchain::database::common
{
class SyncPrivate
{
};

Sync::Sync(
    const api::Session&,
    storage::lmdb::Database&,
    const std::filesystem::path&) noexcept(false)
    : imp_(std::make_unique<SyncPrivate>())
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

auto Sync::Store(const network::otdht::SyncData&, Chain) const noexcept -> bool
{
    return {};
}

auto Sync::Tip(const Chain) const noexcept -> Height { return -1; }

Sync::~Sync() = default;
}  // namespace opentxs::blockchain::database::common
