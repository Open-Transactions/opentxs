// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

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
}  // namespace block
}  // namespace bitcoin

namespace block
{
namespace internal
{
struct Block;
}  // namespace internal

class Hash;
class Header;
}  // namespace block
}  // namespace blockchain

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Block
{
public:
    virtual auto asBitcoin() const noexcept -> const bitcoin::block::Block& = 0;
    virtual auto Header() const noexcept -> const block::Header& = 0;
    virtual auto ID() const noexcept -> const block::Hash& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Block& = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    virtual auto Serialize(Writer&& bytes) const noexcept -> bool = 0;

    virtual auto asBitcoin() noexcept -> bitcoin::block::Block& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Block& = 0;

    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    virtual ~Block() = default;

protected:
    Block() noexcept = default;
};
}  // namespace opentxs::blockchain::block
