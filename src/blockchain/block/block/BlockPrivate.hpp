// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/blockchain/block/Block.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocator.hpp"
#include "util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
class BlockPrivate;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class BlockPrivate : virtual public internal::Block,
                     public opentxs::implementation::Allocated
{
public:
    [[nodiscard]] static auto Blank(alloc::Strategy alloc) noexcept
        -> BlockPrivate*
    {
        return default_construct<BlockPrivate>({alloc.result_});
    }
    static auto Reset(block::Block& header) noexcept -> void;

    virtual auto asBitcoinPrivate() const noexcept
        -> const bitcoin::block::BlockPrivate*;
    virtual auto asBitcoinPublic() const noexcept
        -> const bitcoin::block::Block&;
    [[nodiscard]] virtual auto clone(allocator_type alloc) const noexcept
        -> BlockPrivate*
    {
        return pmr::clone(this, {alloc});
    }

    virtual auto asBitcoinPrivate() noexcept -> bitcoin::block::BlockPrivate*;
    virtual auto asBitcoinPublic() noexcept -> bitcoin::block::Block&;
    [[nodiscard]] virtual auto get_deleter() noexcept -> std::function<void()>
    {
        return make_deleter(this);
    }

    BlockPrivate(allocator_type alloc) noexcept;
    BlockPrivate() = delete;
    BlockPrivate(const BlockPrivate& rhs, allocator_type alloc) noexcept;
    BlockPrivate(const BlockPrivate&) = delete;
    BlockPrivate(BlockPrivate&&) = delete;
    auto operator=(const BlockPrivate&) -> BlockPrivate& = delete;
    auto operator=(BlockPrivate&&) -> BlockPrivate& = delete;

    ~BlockPrivate() override;
};
}  // namespace opentxs::blockchain::block
