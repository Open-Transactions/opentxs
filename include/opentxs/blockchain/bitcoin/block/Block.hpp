// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Block.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class BlockPrivate;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Block : virtual public blockchain::block::Block
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Block&;

    OPENTXS_NO_EXPORT Block(blockchain::block::BlockPrivate* imp) noexcept;
    Block(allocator_type alloc = {}) noexcept;
    Block(const Block& rhs, allocator_type alloc = {}) noexcept;
    Block(Block&& rhs) noexcept;
    Block(Block&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Block& rhs) noexcept -> Block&;
    auto operator=(Block&& rhs) noexcept -> Block&;

    ~Block() override;
};
}  // namespace opentxs::blockchain::bitcoin::block
