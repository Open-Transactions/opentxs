// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/bitcoin/Block.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block
namespace node
{
namespace internal
{
struct BlockOracle;
}  // namespace internal
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace node
{
class OPENTXS_EXPORT BlockOracle
{
public:
    using BitcoinBlock = block::bitcoin::Block;
    using BitcoinBlock_p = std::shared_ptr<const BitcoinBlock>;
    using BitcoinBlockFuture = std::shared_future<BitcoinBlock_p>;
    using BlockHashes = std::pmr::vector<block::pHash>;
    using BitcoinBlockFutures = std::pmr::vector<BitcoinBlockFuture>;

    virtual auto Tip() const noexcept -> block::Position = 0;
    virtual auto DownloadQueue() const noexcept -> std::size_t = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::BlockOracle& = 0;
    virtual auto LoadBitcoin(const block::Hash& block) const noexcept
        -> BitcoinBlockFuture = 0;
    virtual auto LoadBitcoin(const BlockHashes& hashes) const noexcept
        -> BitcoinBlockFutures = 0;
    virtual auto Validate(const BitcoinBlock& block) const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual ~BlockOracle() = default;

protected:
    BlockOracle() noexcept = default;

private:
    BlockOracle(const BlockOracle&) = delete;
    BlockOracle(BlockOracle&&) = delete;
    auto operator=(const BlockOracle&) -> BlockOracle& = delete;
    auto operator=(BlockOracle&&) -> BlockOracle& = delete;
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
