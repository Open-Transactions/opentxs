// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/node/BlockOracle.hpp"  // IWYU pragma: associated

#include <string_view>

#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::implementation
{
BlockOracle::Cache::Mem::Mem(const std::size_t limit) noexcept
    : limit_(limit)
    , queue_()
    , index_()
{
}

auto BlockOracle::Cache::Mem::clear() noexcept -> void
{
    index_.clear();
    queue_.clear();
}

auto BlockOracle::Cache::Mem::find(const ReadView& id) const noexcept
    -> BitcoinBlockFuture
{
    if ((nullptr == id.data()) || (0 == id.size())) { return {}; }

    try {

        return index_.at(id)->second;
    } catch (...) {

        return {};
    }
}

auto BlockOracle::Cache::Mem::push(
    block::pHash&& id,
    BitcoinBlockFuture&& future) noexcept -> void
{
    if (0 == id->size()) { return; }

    auto i = queue_.emplace(queue_.end(), std::move(id), std::move(future));
    const auto& item = *i;
    const auto [j, added] = index_.try_emplace(item.first->Bytes(), &item);

    OT_ASSERT(added);

    while (queue_.size() > limit_) {
        const auto& item = queue_.front();
        index_.erase(item.first->Bytes());
        queue_.pop_front();
    }
}
}  // namespace opentxs::blockchain::node::implementation
