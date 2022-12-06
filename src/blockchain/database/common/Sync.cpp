// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Sync.hpp"  // IWYU pragma: associated

#include <memory>

#include "blockchain/database/common/SyncPrivate.hpp"

namespace opentxs::blockchain::database::common
{
Sync::Sync(
    const api::Session& api,
    storage::lmdb::Database& lmdb,
    const std::filesystem::path& path) noexcept(false)
    : imp_(std::make_unique<SyncPrivate>(api, lmdb, path))
{
}

auto Sync::Load(const Chain chain, const Height height, Message& output)
    const noexcept -> bool
{
    return imp_->Load(chain, height, output);
}

auto Sync::Reorg(const Chain chain, const Height height) const noexcept -> bool
{
    return imp_->Reorg(chain, height);
}

auto Sync::Store(const network::otdht::SyncData& items, Chain chain)
    const noexcept -> bool
{
    return imp_->Store(items, chain);
}

auto Sync::Tip(const Chain chain) const noexcept -> Height
{
    return imp_->Tip(chain);
}

Sync::~Sync() = default;
}  // namespace opentxs::blockchain::database::common
