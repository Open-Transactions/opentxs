// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "blockchain/node/blockoracle/Shared.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/blockchain/block/Validator.hpp"
#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::internal
{
BlockOracle::Shared::Shared(
    const api::Session& api,
    const node::Manager& node,
    allocator_type alloc) noexcept
    : api_(api)
    , node_(node)
    , submit_endpoint_(network::zeromq::MakeArbitraryInproc(alloc.resource()))
    , cache_(api_, node_)
    , db_(node_.Internal().DB())
    , validator_(get_validator(node_.Internal().Chain(), node_.HeaderOracle()))
    , block_fetcher_(std::nullopt)
{
    OT_ASSERT(validator_);

    switch (node_.Internal().GetConfig().profile_) {
        case BlockchainProfile::mobile:
        case BlockchainProfile::desktop:
        case BlockchainProfile::desktop_native: {
        } break;
        case BlockchainProfile::server: {
            block_fetcher_.emplace(api, node);
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())("invalid profile").Abort();
        }
    }
}

auto BlockOracle::Shared::GetBlockBatch(alloc::Default alloc) const noexcept
    -> BlockBatch
{
    return cache_.GetBlockBatch(alloc);
}

auto BlockOracle::Shared::GetBlockJob(alloc::Default alloc) const noexcept
    -> BlockBatch
{
    if (block_fetcher_.has_value()) {

        return block_fetcher_->GetJob(alloc);
    } else {

        return {};
    }
}

auto BlockOracle::Shared::get_allocator() const noexcept -> allocator_type
{
    return cache_.get_allocator();
}

auto BlockOracle::Shared::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockResult
{
    auto output = cache_.Request(block);

    return output;
}

auto BlockOracle::Shared::LoadBitcoin(
    const Vector<block::Hash>& hashes) const noexcept -> BitcoinBlockResults
{
    auto output = cache_.Request(hashes);

    OT_ASSERT(hashes.size() == output.size());

    return output;
}

auto BlockOracle::Shared::StartDownloader(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    cache_.Start(api, node);

    if (block_fetcher_.has_value()) {
        block_fetcher_->Init(std::move(api), std::move(node));
    }
}

auto BlockOracle::Shared::SubmitBlock(
    std::shared_ptr<const bitcoin::block::Block> in) const noexcept -> bool
{
    return cache_.ReceiveBlock(std::move(in));
}

auto BlockOracle::Shared::Tip() const noexcept -> block::Position
{
    return db_.BlockTip();
}

auto BlockOracle::Shared::Validate(
    const bitcoin::block::Block& block) const noexcept -> bool
{
    return validator_->Validate(block);
}

BlockOracle::Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::internal
