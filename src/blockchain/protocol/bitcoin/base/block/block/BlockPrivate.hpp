// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/block/block/BlockPrivate.hpp"

#include "internal/blockchain/protocol/bitcoin/base/block/Block.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Block.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
class BlockPrivate : virtual public blockchain::block::BlockPrivate,
                     virtual public internal::Block
{
public:
    [[nodiscard]] static auto Blank(allocator_type alloc) noexcept
        -> BlockPrivate*
    {
        return pmr::default_construct<BlockPrivate>({alloc});
    }

    auto asBitcoinPrivate() const noexcept
        -> const protocol::bitcoin::base::block::BlockPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() const noexcept
        -> const protocol::bitcoin::base::block::Block& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::BlockPrivate* override
    {
        return pmr::clone_as<blockchain::block::BlockPrivate>(this, {alloc});
    }

    auto asBitcoinPrivate() noexcept
        -> protocol::bitcoin::base::block::BlockPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() noexcept
        -> protocol::bitcoin::base::block::Block& final
    {
        return self_;
    }
    [[nodiscard]] auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }

    BlockPrivate(allocator_type alloc) noexcept;
    BlockPrivate() = delete;
    BlockPrivate(const BlockPrivate& rhs, allocator_type alloc) noexcept;
    BlockPrivate(const BlockPrivate&) = delete;
    BlockPrivate(BlockPrivate&&) = delete;
    auto operator=(const BlockPrivate&) -> BlockPrivate& = delete;
    auto operator=(BlockPrivate&&) -> BlockPrivate& = delete;

    ~BlockPrivate() override;

protected:
    block::Block self_;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
