// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/node/blockoracle/MemDB.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <future>
#include <memory>

#include "internal/blockchain/block/Block.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::blockoracle
{
MemDB::MemDB(const std::size_t limit, allocator_type alloc) noexcept
    : limit_(limit)
    , bytes_(0)
    , queue_(alloc)
    , index_(alloc)
{
}

auto MemDB::clear() noexcept -> void
{
    index_.clear();
    queue_.clear();
    bytes_ = 0;
}

auto MemDB::find(const ReadView id) const noexcept -> BitcoinBlockResult
{
    if (false == valid(id)) {
        LogError()(OT_PRETTY_CLASS())("invalid block id").Flush();

        return {};
    }

    if (auto i = index_.find(id); index_.end() != i) {

        return i->second->second;
    } else {

        return {};
    }
}

auto MemDB::push(block::Hash&& id, BitcoinBlockResult&& future) noexcept -> void
{
    if (id.IsNull()) {
        LogError()(OT_PRETTY_CLASS())("invalid block id").Flush();

        return;
    }

    if (0u < index_.count(id.Bytes())) {
        LogError()(OT_PRETTY_CLASS())("block ")(id.asHex())(" already cached")
            .Flush();

        return;
    }

    OT_ASSERT(future.valid());

    const auto& pBlock = future.get();

    OT_ASSERT(pBlock);

    const auto& block = *pBlock;
    const auto& item = queue_.emplace_back(std::move(id), std::move(future));
    bytes_ += block.Internal().CalculateSize();
    index_.try_emplace(item.first.Bytes(), &item);

    while ((bytes_ > limit_) && (0u < queue_.size())) {
        const auto& item = queue_.front();
        const auto& id = item.first;
        LogTrace()(OT_PRETTY_CLASS())("dropping oldest block ")(id.asHex())(
            " from cache due to exceeding byte limit")
            .Flush();
        index_.erase(item.first.Bytes());
        bytes_ -= item.second.get()->Internal().CalculateSize();
        queue_.pop_front();
    }
}
}  // namespace opentxs::blockchain::node::blockoracle
