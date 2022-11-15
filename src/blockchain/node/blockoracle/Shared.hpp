// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_plain_guarded.h>
#include <cs_shared_guarded.h>
#include <memory>
#include <optional>
#include <shared_mutex>

#include "blockchain/node/blockoracle/cache/Cache.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/BlockFetcher.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Block;
}  // namespace database

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
class Validator;
}  // namespace block

namespace node
{
namespace internal
{
class BlockBatch;
}  // namespace internal

class HeaderOracle;
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class BlockOracle::Shared final : public Allocated
{
public:
    const api::Session& api_;
    const node::Manager& node_;
    const CString submit_endpoint_;
    mutable blockoracle::Cache cache_;

    auto GetBlockBatch(alloc::Default alloc) const noexcept -> BlockBatch;
    auto GetBlockJob(alloc::Default alloc) const noexcept -> BlockBatch;
    auto get_allocator() const noexcept -> allocator_type final;
    auto LoadBitcoin(const block::Hash& block) const noexcept
        -> BitcoinBlockResult;
    auto LoadBitcoin(const Vector<block::Hash>& hashes) const noexcept
        -> BitcoinBlockResults;
    auto SubmitBlock(
        std::shared_ptr<const bitcoin::block::Block> in) const noexcept -> bool;
    auto Tip() const noexcept -> block::Position;
    auto Validate(const bitcoin::block::Block& block) const noexcept -> bool;

    auto StartDownloader(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void;

    Shared(
        const api::Session& api,
        const node::Manager& node,
        allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;

private:
    using OptionalFetcher = std::optional<blockoracle::BlockFetcher>;

    const database::Block& db_;
    const std::unique_ptr<const block::Validator> validator_;
    OptionalFetcher block_fetcher_;

    static auto get_validator(
        const blockchain::Type chain,
        const node::HeaderOracle& headers) noexcept
        -> std::unique_ptr<const block::Validator>;
};
}  // namespace opentxs::blockchain::node::internal
