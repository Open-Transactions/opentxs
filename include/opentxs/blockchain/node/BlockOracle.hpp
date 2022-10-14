// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
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
class Hash;
class Position;
}  // namespace block

namespace node
{
namespace internal
{
class BlockOracle;
}  // namespace internal
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT BlockOracle
{
public:
    virtual auto DownloadQueue() const noexcept -> std::size_t = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::BlockOracle& = 0;
    virtual auto LoadBitcoin(const block::Hash& block) const noexcept
        -> BitcoinBlockResult = 0;
    virtual auto LoadBitcoin(const Vector<block::Hash>& hashes) const noexcept
        -> BitcoinBlockResults = 0;
    virtual auto Tip() const noexcept -> block::Position = 0;
    virtual auto Validate(const bitcoin::block::Block& block) const noexcept
        -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::BlockOracle& = 0;

    BlockOracle(const BlockOracle&) = delete;
    BlockOracle(BlockOracle&&) = delete;
    auto operator=(const BlockOracle&) -> BlockOracle& = delete;
    auto operator=(BlockOracle&&) -> BlockOracle& = delete;

    OPENTXS_NO_EXPORT virtual ~BlockOracle() = default;

protected:
    BlockOracle() noexcept = default;
};
}  // namespace opentxs::blockchain::node
