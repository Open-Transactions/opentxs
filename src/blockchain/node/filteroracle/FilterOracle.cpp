// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/FilterOracle.hpp"  // IWYU pragma: associated

#include <string_view>

#include "blockchain/node/filteroracle/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainFilterOracle(
    const api::Session& api,
    const blockchain::node::Manager& node,
    const blockchain::cfilter::Type filter) noexcept
    -> std::unique_ptr<blockchain::node::FilterOracle>
{
    using ReturnType = opentxs::blockchain::node::implementation::FilterOracle;

    return std::make_unique<ReturnType>(api, node, filter);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node::implementation
{
FilterOracle::FilterOracle(
    const api::Session& api,
    const node::Manager& node,
    const blockchain::cfilter::Type filter) noexcept
    : internal::FilterOracle()
    , shared_p_(std::make_shared<filteroracle::Shared>(api, node, filter))
    , shared_(*shared_p_)
{
    OT_ASSERT(shared_p_);

    shared_.api_.Network().Asio().Internal().Post(
        ThreadPool::General,
        [shared = shared_p_] { shared->Init(); },
        CString(print(node.Internal().Chain()))
            .append(" filter oracle initialization"));
}

auto FilterOracle::FilterTip(const cfilter::Type type) const noexcept
    -> block::Position
{
    return shared_.CfilterTip(type);
}

auto FilterOracle::DefaultType() const noexcept -> cfilter::Type
{
    return shared_.default_type_;
}

auto FilterOracle::GetFilterJob() const noexcept -> CfilterJob
{
    return shared_.GetFilterJob();
}

auto FilterOracle::GetHeaderJob() const noexcept -> CfheaderJob
{
    return shared_.GetHeaderJob();
}

auto FilterOracle::Heartbeat() noexcept -> void { shared_.Heartbeat(); }

auto FilterOracle::Init(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept -> void
{
    shared_.Init(api, node, shared_p_);
}

auto FilterOracle::LoadFilter(
    const cfilter::Type type,
    const block::Hash& block,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> GCS
{
    return shared_.LoadCfilter(type, block.Bytes(), alloc, monotonic);
}

auto FilterOracle::LoadFilters(
    const cfilter::Type type,
    const Vector<block::Hash>& blocks,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Vector<GCS>
{
    return shared_.LoadCfilters(type, blocks, alloc, monotonic);
}

auto FilterOracle::LoadFilterHeader(
    const cfilter::Type type,
    const block::Hash& block) const noexcept -> cfilter::Header
{
    return shared_.LoadCfheader(type, block);
}

auto FilterOracle::ProcessBlock(
    const bitcoin::block::Block& block,
    alloc::Default monotonic) const noexcept -> bool
{
    return shared_.ProcessBlock(block, monotonic);
}

auto FilterOracle::ProcessSyncData(
    const block::Hash& prior,
    const Vector<block::Hash>& hashes,
    const network::otdht::Data& data,
    alloc::Default monotonic) const noexcept -> void
{
    shared_.ProcessSyncData(prior, hashes, data, monotonic);
}

auto FilterOracle::Shutdown() noexcept -> void { shared_.Shutdown(); }

auto FilterOracle::Start() noexcept -> void { shared_.Start(); }

auto FilterOracle::Tip(const cfilter::Type type) const noexcept
    -> block::Position
{
    return shared_.CfilterTip(type);
}

FilterOracle::~FilterOracle() { Shutdown(); }
}  // namespace opentxs::blockchain::node::implementation
