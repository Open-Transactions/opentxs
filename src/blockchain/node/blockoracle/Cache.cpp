// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Cache.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::blockoracle
{
Cache::Cache(allocator_type alloc) noexcept
    : data_(alloc)
    , index_(alloc)
    , reverse_(alloc)
    , size_(0_uz)
{
}

auto Cache::add(const block::Hash& id, ReadView bytes) noexcept -> CachedBlock
{
    if (auto it = index_.find(id); index_.end() != it) {

        return *it->second;
    } else {
        auto i = data_.insert(data_.end(), std::make_shared<ByteArray>(bytes));
        auto [j, rc1] = index_.try_emplace(id, i);

        assert_true(rc1);

        auto [_, rc2] = reverse_.try_emplace(i, j);

        assert_true(rc2);

        auto out = *i;

        assert_false(nullptr == out);
        size_ += out->size();

        return *i;
    }
}

auto Cache::Clear() noexcept -> void
{
    reverse_.clear();
    index_.clear();
    data_.clear();
}

auto Cache::get_allocator() const noexcept -> allocator_type
{
    return data_.get_allocator();
}

auto Cache::Load(const block::Hash& id) noexcept -> CachedBlock
{
    if (auto i = index_.find(id); index_.end() != i) {

        return *i->second;
    } else {

        return {};
    }
}

auto Cache::remove(Data::iterator i) noexcept -> Data::iterator
{
    if (auto r = reverse_.find(i); reverse_.end() != r) {
        size_ -= (*i)->size();
        index_.erase(r->second);
        reverse_.erase(r);

        return data_.erase(i);
    } else {
        LogAbort()().Abort();
    }
}

auto Cache::Store(const block::Hash& id, ReadView bytes) noexcept -> CachedBlock
{
    auto out = add(id, bytes);
    auto i = data_.begin();

    while ((size_ > limit_) && (i != data_.end())) { i = remove(i); }

    return out;
}

Cache::~Cache() = default;
}  // namespace opentxs::blockchain::node::blockoracle
