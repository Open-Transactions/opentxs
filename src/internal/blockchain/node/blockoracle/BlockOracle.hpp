// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Position.hpp"

#pragma once

#include <cstddef>
#include <memory>
#include <span>

#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{

namespace block
{
class Block;
class Hash;
}  // namespace block

namespace node
{
namespace internal
{
class BlockBatch;
}  // namespace internal

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class BlockOracle final : public node::BlockOracle
{
public:
    class Actor;
    class Shared;

    auto BlockExists(const block::Hash& block) const noexcept -> bool;
    auto DownloadQueue() const noexcept -> std::size_t;
    auto FetchAllBlocks() const noexcept -> bool;
    auto GetWork(alloc::Default alloc) const noexcept -> BlockBatch;
    auto Internal() const noexcept -> const BlockOracle& final { return *this; }
    auto Load(const block::Hash& block) const noexcept -> BlockResult final;
    auto Load(std::span<const block::Hash> hashes) const noexcept
        -> BlockResults final;
    auto SubmitBlock(
        const blockchain::block::Block& in,
        alloc::Default monotonic) const noexcept -> bool;
    auto Tip() const noexcept -> block::Position final;

    auto Internal() noexcept -> BlockOracle& final { return *this; }
    auto Start(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void;

    BlockOracle() noexcept;
    BlockOracle(const BlockOracle&) = delete;
    BlockOracle(BlockOracle&& rhs) noexcept;
    auto operator=(const BlockOracle&) -> BlockOracle& = delete;
    auto operator=(BlockOracle&&) -> BlockOracle& = delete;

    ~BlockOracle() final;

private:
    std::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::blockchain::node::internal
