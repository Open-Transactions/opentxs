// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Position.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

#include "internal/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

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
class Block;
class Hash;
class Position;
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

    auto DownloadQueue() const noexcept -> std::size_t;
    auto GetWork(alloc::Default alloc) const noexcept -> BlockBatch;
    auto Internal() const noexcept -> const BlockOracle& final { return *this; }
    auto Load(const block::Hash& block) const noexcept
        -> BitcoinBlockResult final;
    auto Load(std::span<const block::Hash> hashes) const noexcept
        -> BitcoinBlockResults final;
    auto SubmitBlock(const blockchain::block::Block& in) const noexcept -> bool;
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
    // TODO switch to std::shared_ptr once the android ndk ships a version of
    // libc++ with unfucked pmr / allocate_shared support
    boost::shared_ptr<Shared> shared_;
};
}  // namespace opentxs::blockchain::node::internal
