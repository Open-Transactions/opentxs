// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>

#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "util/ByteLiterals.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class Cache final : public opentxs::Allocated
{
public:
    auto get_allocator() const noexcept -> allocator_type final;

    auto Load(const block::Hash& id) noexcept -> CachedBlock;

    auto Clear() noexcept -> void;
    auto Store(const block::Hash& id, ReadView bytes) noexcept -> CachedBlock;

    Cache(allocator_type alloc) noexcept;
    Cache() = delete;
    Cache(const Cache&) = delete;
    Cache(Cache&&) = delete;
    auto operator=(const Cache&) -> Cache& = delete;
    auto operator=(Cache&&) -> Cache& = delete;

    ~Cache() final;

private:
    using Data = Deque<std::shared_ptr<const ByteArray>>;
    using Index = Map<block::Hash, Data::iterator>;
    using Reverse = Map<Data::iterator, Index::iterator>;

    static constexpr auto limit_ = 64_mib;

    Data data_;
    Index index_;
    Reverse reverse_;
    std::size_t size_;

    auto add(const block::Hash& id, ReadView bytes) noexcept -> CachedBlock;
    auto remove(Data::iterator i) noexcept -> Data::iterator;
};
}  // namespace opentxs::blockchain::node::blockoracle
