// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/block/block/BlockPrivate.hpp"

#include <functional>

#include "internal/blockchain/bitcoin/block/Block.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::bitcoin::block
{
class BlockPrivate : virtual public blockchain::block::BlockPrivate,
                     virtual public internal::Block
{
public:
    [[nodiscard]] static auto Blank(alloc::Strategy alloc) noexcept
        -> BlockPrivate*
    {
        return default_construct<BlockPrivate>({alloc.result_});
    }

    auto asBitcoinPrivate() const noexcept
        -> const bitcoin::block::BlockPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() const noexcept -> const bitcoin::block::Block& final
    {
        return self_;
    }
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::BlockPrivate* override
    {
        return pmr::clone_as<blockchain::block::BlockPrivate>(this, {alloc});
    }

    auto asBitcoinPrivate() noexcept -> bitcoin::block::BlockPrivate* final
    {
        return this;
    }
    auto asBitcoinPublic() noexcept -> bitcoin::block::Block& final
    {
        return self_;
    }
    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> override
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

protected:
    block::Block self_;
};
}  // namespace opentxs::blockchain::bitcoin::block
